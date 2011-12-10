#ifndef VASSILIS_AUXILIARIES_H
#define VASSILIS_AUXILIARIES_H

#include "vplatform.h"

#include <assert.h>
#include <cstdarg>
#include <stdio.h>
#include <list>
#include <vector>
#include <math.h>


#include <map>

#include "v_types.h"
#include "general_templates.h"

class class_file;

class vPoint
{
public:
 vint8 row, col;
 vPoint() {}
 vPoint(vint8 in_row, vint8 in_col)
 {
   row = in_row;
   col = in_col;
 }
};

typedef vPoint class_point;

// like printf, but it flushes the output.
int vPrintAndFlush(const char * format, ...);

// like scanf, but it doesn't give up until all fields have been
// successfully matched.
int vScanAndVerify(const char * format, ...);

// like scanf, but it doesn't give up until all fields have been
// successfully matched.
int vVScanAndVerify(const char * format, va_list arguments);

// Like fscanf, but it takes a va_list as an argument (this comes with C++
// in some platforms, called vfscanf)
int vFscan(FILE * fp, const char * format, va_list arguments);

// Get a non-empty line from a file. If the first non-empty line
// found exceeds max_length, only the first max_length characters
// are returned. It returns the length of the string (with the
// newline character removed).
vint8 vFileGetNonEmptyLine(FILE * fp, char * buffer, vint8 max_length);

vint8 vShellGetLine(char * buffer, vint8 max_length);

vint8 vGetNonEmptyLine(char * buffer, vint8 max_length);


class Bitmap
{
private:
 vint8 size;
 vint8 byte_size;
 uchar * data;


 // I use buffer as a continuous memory chunk, that first contains */
 // the value of size and then it contains the data. I thought that */
 // might be useful if I wanted all bitmaps in the HuffmanCodedData */
 // class to be contiguous in memory, so that we could Huffman encode */
 // data that is already Huffman encoded. Eventually I changed my */
 // mind about Huffman encoding implementation, so maybe I should get */
 // rid of buffer and the constructor that takes a buffer as an input */
 uchar * buffer;

 ushort delete_buffer; // Zero if buffer was not allocated by constructor.

public:
 Bitmap(vint8 the_size);
 Bitmap(vint8 the_size, uchar * the_buffer);
 ~Bitmap();
 void Save(const char * filename);
 void Save(FILE * fp);
 static Bitmap * Load(const char * filename);
 static Bitmap * Load(FILE * fp);
 vint8 LoadData(FILE * fp);
 void Write(vint8 index, uchar value);
 uchar Read(vint8 index);
 void Print();
 vint8 ByteSize();
 vint8 BufferSize();
 vint8 Size();
 const uchar * GetData();
 uchar * WritableData();
};


class vBoundingBox
{
public:
 vint8 top, left, bottom, right;
 // An uninitialized bounding box should have invalid values:
 // bottom < top, right < left.
 vBoundingBox()
 {
   top = 0;
   left = 0;
   bottom = -1;
   right = -1;
 }

 vBoundingBox(vint8 in_top, vint8 in_bottom, vint8 in_left, vint8 in_right)
 {
   top = in_top;
   bottom = in_bottom;
   left = in_left;
   right = in_right;
 }

 inline vint8 Height()
 {
   return bottom - top + 1;
 }

 inline vint8 Width()
 {
   return right - left + 1;
 }

 inline vint8 MaxSide()
 {
   return Max(Height(), Width());
 }

 inline vint8 MinSide()
 {
   return Min(Height(), Width());
 }

 inline vint8 CenterRow()
 {
   return (top + bottom) / 2;
 }

 inline vint8 CenterCol()
 {
   return (left + right) / 2;
 }

 inline void Print()
 {
   vPrint("\ncenter = (%.1lf, %.1lf), top = %li, bottom = %li, left = %li, \
            right = %li, height = %li, width = %li\n",
           (double) (top + bottom + 0.0) / 2.0, (double) (left + right + 0.0) / 2.0,
           top, bottom, left, right, bottom - top + 1, right - left + 1);
 }

};


// The class Label was created to label connected components
// of an image.
class Label : public vBoundingBox
{
public:
 vint8 id;
 vint8 count;

 // r, g, b are colors that we assign to the label.
 // this is used to generate an RGB image where
 // each label gets its own color
 uchar r;
 uchar g;
 uchar b;

 // value is the value of the connected component
 // to which the label corresponds
 uchar value;

 Label();
};

typedef Label class_label;

class LabelMore
{
public:
 ushort operator() (const Label & label1, const Label & label2);
};

class LabelBoxMore
{
public:
 ushort operator() (const Label & label1, const Label & label2);
};

ushort vBoxesOverlap(vBoundingBox & b1, vBoundingBox & b2);


class vDate
{
public:
 vint8 year;
 vint8 month;
 vint8 day;
 vint8 hour;
 vint8 minute;
 vint8 second;
 vint8 millisecond;

 ushort Read(FILE * fp);
 void Print(FILE * fp);
};


// Class vStringTable is an inefficient but simple way to create tables
// that map strings to objects. I could have used the STL map class instead,
// hopefully this class will lead to simple code. I intend to use it
// only for tables that will contain a few objects, and where efficiency is
// not really an issue.

class vStringTable
{
private:
 std::vector<char *> keys;
 std::vector<void *> objects;

public:
 vStringTable();
 ~vStringTable();

 // Removes all keys and objects, so that this table becomes empty.
 vint8 Clear();

 // Store object under key. Fail if key is already used.
 vint8 Put(const char * key, void * object);

 // Return object stored under given key
 void * Get(const char * key);

 // Remove object stored under given key, and return it.
 void * Remove(const char * key);

 // Return the number of objects stored.
 vint8 Size();

 // Return the i-th object stored.
 void * GetIndex(vint8 index);

 // Remove the i-th object, and return it.
 void * RemoveIndex(vint8 index);

 // Returns true if the key is being used already, false
 // otherwise.
 vint8 KeyExists(const char * key);

 // Returns the number of keys in the table that are mapped to
 // the given object.
 vint8 CountKeys(void * object);

 // It returns some key that is mapped to the given object. If no
 // such key exists, then it returns 0.
 const char * GetKey(void * object);
};


void vCheckArgs(int arg, int low, int high, const char * error);
short vIsGIF(const char * filename);
short vIsJPEG(const char * filename);
short vIsPPM(const char * filename);
short vIsPGM(const char * filename);

vint8 vFilesize(const char * filename);
unsigned short vCopyFile(const char * source, const char * target);
short vStringEndsIn(const char * string, const char * ending);
short vStringCaseEndsIn(const char * string, const char * ending);

void vAdjustDirectoryName(char * directory);

vint8 vGetNumber(char * directory);

void vIncreaseNumber(char * directory);

void vAddArrays(vint8 * array1, vint8 * array2, vint8 size);

void vSubtractArrays(vint8 * array1, vint8 * array2, vint8 size);

char * vAdjustEnding2(const char * source, const char * ending);

void vAdjustEnding3(const char * source, char * target, const char * ending);

char * vCopyString(const char * source);

char * vCopyString(const char * source, vint8 start, vint8 end);

// Replaces every occurrence of old_char in str with new_char.
vint8 vReplaceCharacter(char * str, char old_char, char new_char);

char * string_from_number(vint8 number);

// this function adds zeros to the beginning of the string, so that
// the length of the result matches the argument "length".
char *  string_from_number(vint8 number, vint8 length);

char * vMergeStrings2(const char * first, const char * second);

char * vMergeStrings3(const char * first, const char * second,
                      const char * third);

char * vMergeStrings4(const char * first, const char * second,
                      const char * third, const char * fourth);

char * vMergeStrings5(const char * first, const char * second,
                      const char * third, const char * fourth,
                      const char * fifth);

// It returns the shortest ending substring of filename that
// starts with the '.' character.
char * vGetEnding(const char * filename);

// It returns result of removing the filename's ending
// from the filename (see comments before vGetEnding on
// how the "ending" of a file is defined). The difference
// between this function and vFileTitle is that vFileTitle
// discards the path part of the filename.
char * vRemoveEnding(const char * filename);

char * vFileTitle(const char * filename);


// If name ends in old_ending, we return a copy of the name, but
// adjusted so that the old ending is replaced by the new
// ending. If name doesn't end in old_ending, then we just append
// the new ending to it.
char * vReplaceEnding3(const char * name, const char * old_ending,
                      const char * new_ending);

// vReplaceEndings2 finds the ending of the name(defined as the
// smallest ending substring of name that starts with '.') and
// replaces it with new_ending  (which should start with the
// character '.' if we want to have a dot before the ending in
// the result).
char * vReplaceEnding2(const char * name, const char * new_ending);

char * vParentDir(const char * filename, const char * dir_separator);

char * vSimpleName(const char * filename, const char * separator);

ushort vPathComponents(const char * filename, std::vector<voidp> * components);

unsigned short vFileExists(const char *filename);

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
ushort vDirectoryExists(const char * filename);

ulong vMaskedValue(ulong number, ulong mask);

uchar vMaskShift(ulong mask);

void vEraseCharpVector(std::vector<voidp> * strings);

void vEraseCharpVector(std::vector<charp> * strings);

static inline void function_erase_string_vector(std::vector<charp> * strings)
{
 vEraseCharpVector(strings);
}

vint4 vHostToPCLong(long number);

ushort vHostToPCUshort(ushort number);

vint4 vPCToHostLong(vint4 number);

vint4 vUnixToHostLong(vint4 number);

ushort vPCToHostUshort(ushort number);

void vGetVint8Bytes(vint8 a, uchar * buffer);

vint8 vReadPCUshorts(FILE * fp, ushort * numbers, vint8 items);

vint8 vReadUnixUshorts(FILE * fp, ushort * numbers, vint8 items);

vint8 vReadUnixLong(FILE * fp, long * number);

vint8 vReadUnixLongs(FILE * fp, long * numbers, vint8 items);


// alex & v: this is readPCLong
vint8 read_integer(FILE * fp, integer * number);

vint8 read_integer(class_file * fp, integer * number);

// alex & v: this is readPCLongs
vint8 read_integers(FILE * fp, integer * numbers, vint8 items);

vint8 read_integers(class_file * fp, integer * numbers, vint8 items);

vint8 read_vint8(FILE * fp, vint8 * number);

vint8 read_vint8(class_file * fp, vint8 * number);

vint8 read_vint8s(FILE * fp, vint8 * numbers, vint8 items);

vint8 read_vint8s(class_file * fp, vint8 * numbers, vint8 items);

// read_old_longs is useful for reading 32-bit integers into vint8 arrays
// the name comes from the fact that vint8 is a replacement for
// the "long" type, which in old (32-bit) platforms was only a 32-bit integer.
vint8 read_old_longs(FILE * fp, vint8 * numbers, vint8 items);

vint8 read_old_longs(class_file * fp, vint8 * numbers, vint8 items);

vint8 vWritePCUshorts(FILE * fp, ushort * numbers, vint8 items);

vint8 store_integer(FILE * fp, integer number);

vint8 store_integer(class_file * fp, integer number);

vint8 store_integers(FILE * fp, integer * numbers, vint8 items);

vint8 store_integers(class_file * fp, integer * numbers, vint8 items);

vint8 store_vint8(FILE * fp, vint8 number);

vint8 store_vint8(class_file * fp, vint8 number);

vint8 store_vint8s(FILE * fp, const vint8 * numbers, const vint8 items);

vint8 store_vint8s(class_file * fp, const vint8 * numbers, const vint8 items);

// store_old_longs is useful for saving 32-bit integers into vint8 arrays
// the name comes from the fact that vint8 is a replacement for
// the "long" type, which in old (32-bit) platforms was only a 32-bit integer.
vint8 store_old_longs(FILE * fp, const vint8 * numbers, const vint8 items);

vint8 store_old_longs(class_file * fp, const vint8 * numbers, const vint8 items);

long vReadPCDoubles(FILE * fp, double * numbers, vint8 items);

long vWritePCDoubles(FILE * fp, double * numbers, vint8 items);

ushort vFilePatternsMatch(const char * pattern, const char * filename);

void vWaitForEnter();

char * vJoinPaths(const char * path1, const char * path2);

char * vJoinPaths3(const char * path1, const char * path2,
                   const char * path3);

char * vJoinPaths4(const char * path1, const char * path2,
                   const char * path3, const char * path4);

char * vJoinPaths5(const char * path1, const char * path2,
                   const char * path3, const char * path4, const char * path5);

static inline char * vJoinPaths2(const char * path1, const char * path2)
{
 return vJoinPaths(path1, path2);
}

vint8 vTypeToVint8(Type type);

Type vVint8ToType(vint8 number);

vint8 vSizeOfType(Type type);

char * vReadStringText(FILE * fp);

vint8 vWriteStringText(FILE * fp, const char * string);

vint8 vWritePCString(FILE * fp, char * string);

char * vReadPCString(FILE * fp);

// Assuming that all the data in the is numbers, the numbers
// are read and stored in the vector "numbers".
vint8 vReadDoubleVectorB(const char * filename,
                       std::vector<double> * numbers);
vint8 vReadDoubleVectorA(FILE * fp, std::vector<double> * numbers);

// True if the file with the given filename can be opened for writing.
// False otherwise (for example, if the directory specified by the
// filename doesn't exist).
unsigned short vWritableFileExists(char * filename);

// Returns the integer that is the nearest to the number.
// x.5 gets rounded to floor(x) + 1.
//inline vint8 round_number2(double number)
//{
//  double lower = floor(number);
//  double decimal_part = number - lower;
//  if (decimal_part < 0.5)
//  {
//    return lower;
//  }
//  else
//  {
//    return lower+1;
//  }
//}


inline vint8 round_number(double number)
{
 if (number >= 0)
 {
   return (vint8) (number + (double) 0.5);
 }
 else
 {
   return (vint8) (number - (double) 0.5);
 }
}

inline vint8 round_number(vint8 number)
{
 return number;
}


inline vint8 round_number(float number)
{
 if (number >= 0)
 {
   return (vint8) (number + (float) 0.5);
 }
 else
 {
   return (vint8) (number - (float) 0.5);
 }
}


// Length required to write a decimal number with characters. We include
// a character for the minus sign, if the number is negative.
vint8 function_decimal_length(vint8 number);

vint8 function_random_vint8();

// Returns a random number, uniformly distributed between low and high
// (including low and high).
vint8 function_random_vint8(vint8 low, vint8 high);

// Returns a random number, uniformly distributed between low and high
double vRandomDouble(double low, double high);

// exit_error is a handy way to exit when we encounter a condition
// that the code cannot handle. Sometimes that condition is caused by
// bugs, sometimes it is simply stuff I chose not to address to save
// time. Calling this function we get a helpful message. Another very
// useful thing is that, in debug mode, if we set a breakpoint on
// this function, we can easily get to the lines of code that demonstrate
// the problem.
vint8 exit_error(char * format, ...);


// Choose "number" numbers between low and high, and store them
// in result. No repetitions are allowed.
vint8 vPickWithoutRepetitions2(vint8 low, vint8 high, vint8 number,
                              std::vector<vint8> * result);

// class_couple is used so that we can sort an object by some value, but
// still have access to the object itself. This is achieved
// using the function S_PairLess.
class class_couple
{
public:
 float value;
 const void * object;

 class_couple()
 {
 }

 class_couple(const float in_value, const void * in_object)
 {
   value = in_value;
   object = in_object;
 }
};


// class_couple is used so that we can sort an object by some value, but
// still have access to the object itself. This is achieved
// using the function S_PairLess.
template<class object>
class class_sort_couple
{
public:
 float value;
 object entry;

 class_sort_couple()
 {
 }

 class_sort_couple(const float in_value, const object & in_entry)
 {
   value = in_value;
   entry = in_entry;
 }
};


class couple_less
{
public:
 ushort operator() (const class_couple & object1, const class_couple & object2)
 {
   return (object1.value < object2.value);
 }
};


class couple_more
{
public:
 ushort operator() (const class_couple & object1, const class_couple & object2)
 {
   return (object1.value > object2.value);
 }
};


template<class object>
class sort_couple_less
{
public:
 ushort operator() (const class_sort_couple<object> & object1, 
                    const class_sort_couple<object> & object2)
 {
   return (object1.value < object2.value);
 }
};


template<class object>
class sort_couple_more
{
public:
 ushort operator() (const class_sort_couple<object> & object1, 
                    const class_sort_couple<object> & object2)
 {
   return (object1.value > object2.value);
 }
};


// class_string_couple is used so that we can sort an object by some
// string value, but
// still have access to the object itself. This is achieved
// using the function S_PairLess.
class class_string_couple
{
public:
 char* value;
 void * object;

 class_string_couple ()
 {
 }

 class_string_couple (char* in_value, void * in_object)
 {
   value = in_value;
   object = in_object;
 }
};


class string_couple_less
{
public:
 ushort operator() (const class_string_couple & object1, const
class_string_couple & object2)
 {
   return (strcmp (object1.value, object2.value) < 0);
 }
};


vint8 function_make_directory(const char * directory);




/////////////////////////////////////////////////////////////
// alternative speech-recognition friendly function names
/////////////////////////////////////////////////////////////


static inline vint8 function_file_size(const char * filename)
{
 return vFilesize(filename);
}


static inline unsigned short function_copy_file(const char * source,
const char * target)
{
 return vCopyFile(source, target);
}


static inline short function_check_ending(const char * string, const
char * ending)
{
 return vStringEndsIn(string, ending);
}


static inline short function_check_case_ending(const char * string,
const char * ending)
{
 return vStringCaseEndsIn(string, ending);
}

static inline char * function_adjust_ending_two(const char * source,
const char * ending)
{
 return vAdjustEnding2(source, ending);
}


static inline void function_adjust_ending_three(const char * source,
char * target, const char * ending)
{
	vAdjustEnding3(source, target, ending);
}


static inline char * function_copy_string(const char * source)
{
 return vCopyString(source);
}


static inline char * function_copy_string_three(const char * source,
                                                vint8 start, vint8 end)
{
 return vCopyString(source, start, end);
}


// Replaces every occurrence of old_char in str with new_char.
static inline vint8 function_replace_character(char * str, char
old_char, char new_char)
{
 return vReplaceCharacter(str, old_char, new_char);
}


static inline char * function_merge_strings(const char * first, const
char * second)
{
 return vMergeStrings2(first, second);
}


static inline char * function_merge_strings_three(const char * first,
const char * second,
                      const char * third)
{
 return vMergeStrings3(first, second, third);
}


static inline char * function_merge_strings_four(const char * first,
const char * second,
                      const char * third, const char * fourth)
{
 return vMergeStrings4(first, second, third, fourth);
}


static inline char * function_merge_strings_five(const char * first,
const char * second,
                      const char * third, const char * fourth,
                      const char * fifth)
{
 return vMergeStrings5(first, second, third, fourth, fifth);
}


static inline char * function_get_ending(const char * filename)
{
 return vGetEnding(filename);
}


static inline char * function_remove_ending(const char * filename)
{
 return vRemoveEnding(filename);
}


static inline char * function_file_title(const char * filename)
{
 return vFileTitle(filename);
}


static inline char * function_replace_ending_three(const char * name,
const char * old_ending,
                      const char * new_ending)
{
 return vReplaceEnding3(name, old_ending, new_ending);
}


static inline char * function_replace_ending_two(const char * name,
const char * new_ending)
{
 return vReplaceEnding2(name, new_ending);
}


// function_replace_ending finds the ending of the name(defined as the
// smallest ending substring of name that starts with '.') and
// replaces it with new_ending  (which should start with the
// character '.' if we want to have a dot before the ending in
// the result).
static inline char * function_replace_ending(const char * name, const char * new_ending)
{
 return vReplaceEnding2(name, new_ending);
}


static inline char * function_parent_directory(const char * filename,
const char * dir_separator)
{
 return vParentDir(filename, dir_separator);
}


static inline char * function_parent_directory(const char * filename)
{
 return vParentDir(filename, platform_directory_slash);
}


static inline char * function_simple_name(const char * filename, const char * separator)
{
 return vSimpleName(filename, separator);
}

static inline char * function_simple_name(const char * filename)
{
 return vSimpleName(filename, platform_directory_slash);
}


static inline ushort functions_path_components(const char * filename,
std::vector<voidp> * components)
{
 return vPathComponents(filename, components);
}


static inline unsigned short function_file_exists(const char *filename)
{
 return vFileExists(filename);
}


static inline ushort function_directory_exists(const char * filename)
{
 return vDirectoryExists(filename);
}


static inline ulong function_masked_value(ulong number, ulong mask)
{
 return vMaskedValue(number, mask);
}


static inline uchar function_mask_shift(ulong mask)
{
 return vMaskShift(mask);
}


static inline void function_erase_string_vector(std::vector<voidp> * strings)
{
   vEraseCharpVector(strings);
}


static inline long function_Intel_long(long number)
{
 return vHostToPCLong(number);
}


static inline ushort function_Intel_ushort(ushort number)
{
 return vHostToPCUshort(number);
}


static inline vint4 function_Intel_host_long(long number)
{
 return vPCToHostLong(number);
}


static inline long function_UNIX_host_long(long number)
{
 return vUnixToHostLong(number);
}


static inline ushort function_Intel_host_ushort(ushort number)
{
 return vPCToHostUshort(number);
}


static inline void function_get_integer_eight_bytes(vint8 a, uchar * buffer)
{
  vGetVint8Bytes(a, buffer);
}


static inline vint8 function_read_Intel_ushorts(FILE * fp, ushort * numbers, vint8 items)
{
 return vReadPCUshorts(fp, numbers, items);
}

static inline vint8 function_read_UNIX_ushorts(FILE * fp, ushort * numbers, vint8 items)
{
 return vReadUnixUshorts(fp, numbers, items);
}


static inline vint8 function_read_UNIX_long(FILE * fp, long * number)
{
 return vReadUnixLong(fp, number);
}


static inline vint8 function_read_UNIX_longs(FILE * fp, long * numbers, vint8 items)
{
 return vReadUnixLongs(fp, numbers, items);
}


static inline vint8 function_save_Intel_ushorts(FILE * fp, ushort * numbers, vint8 items)
{
 return vWritePCUshorts(fp, numbers, items);
}


static inline vint8 function_read_Intel_doubles(FILE * fp, double * numbers, vint8 items)
{
 return vReadPCDoubles(fp, numbers, items);
}


static inline vint8 function_save_Intel_doubles(FILE * fp, double * numbers, vint8 items)
{
 return vWritePCDoubles(fp, numbers, items);
}


static inline ushort function_file_patterns_match(const char *
pattern, const char * filename)
{
 return vFilePatternsMatch(pattern, filename);
}


static inline void function_wait_for_enter()
{
  vWaitForEnter();
}


static inline char * function_join_paths(const char * path1, const char * path2)
{
 return vJoinPaths(path1, path2);
}


static inline char * function_join_paths_three(const char * path1,
const char * path2,
                   const char * path3)
{
 return vJoinPaths3(path1, path2, path3);
}


static inline char * function_join_paths_four(const char * path1,
const char * path2,
                         const char * path3, const char * path4)
{
 return vJoinPaths4(path1, path2, path3, path4);
}


static inline char * function_join_paths_five(const char * path1,
const char * path2,
                   const char * path3, const char * path4, const char * path5)
{
 return vJoinPaths5(path1, path2, path3, path4, path5);
}


static inline char * function_join_path_two(const char * path1, const
char * path2)
{
 return vJoinPaths2(path1, path2);
}


static inline vint8 function_type_vint8(Type type)
{
 return vTypeToVint8(type);
}


static inline Type function_vint8_type(vint8 number)
{
 return vVint8ToType(number);
}


static inline vint8 function_type_size(Type type)
{
 return vSizeOfType(type);
}


static inline char * load_string_text(FILE * fp)
{
 return vReadStringText(fp);
}


static inline vint8 save_string_text(FILE * fp, const char * string)
{
 return vWriteStringText(fp, string);
}


static inline vint8 function_write_Intel_string(FILE * fp, char * string)
{
 return vWritePCString(fp, string);
}


static inline char * function_read_Intel_string(FILE * fp)
{
 return vReadPCString(fp);
}


// Assuming that all the data in the is numbers, the numbers
// are read and stored in the vector "numbers".
static inline vint8 function_read_double_vectorB(const char * filename,
                                                 std::vector<double> * numbers)
{
 return vReadDoubleVectorB(filename, numbers);
}

static inline vint8 function_read_double_vectorA(FILE * fp, std::vector<double> * numbers)
{
 return vReadDoubleVectorA(fp, numbers);
}


// True if the file with the given filename can be opened for writing.
// False otherwise (for example, if the directory specified by the
// filename doesn't exist).
static inline unsigned short function_writable_file_exists(char * filename)
{
 return vWritableFileExists(filename);
}


// Returns a random number, uniformly distributed between low and high
static inline double function_random_double(double low, double high)
{
 return vRandomDouble(low, high);
}


vint8 exit_error(char * format, ...);

// function_warning is an alternative to calling exit_error, when we
// encounter a condition that we can handle, but which is strong
// evidence that something is wrong. Again, by putting a breakpoint
// on this function, we can easily spot the cases where that happens.
vint8 function_warning(char * format, ...);


// Choose "number" numbers between low and high, and store them
// in result. No repetitions are allowed.
static inline vint8 function_pick_without_repetitions2(vint8 low, vint8 high, vint8 number,
                                                      std::vector<vint8> * result)
{
 return vPickWithoutRepetitions2(low, high, number, result);
}

static inline vint8 function_compare_strings (const char* string1,
const char* string2)
{
 return strcmp (string1, string2);
}

// This class is used so that I can define objects of type
// map<char *, void *, CharpLess>. STL provides a comparator
// object for strings, but not for char * strings.
class CharpLess
{
public:
 ushort operator()(const char *x, const char *y) const
 {
   return (strcmp(x, y) < 0);
 }
};


// this class is aimed to be a substitute for the FILE structure.
// The reason I needed this substitute is that, as of September 30, 2005
// the Linux version that we have on elf and orc cannot handle file sizes
// that are greater than 2 GB.  With existing files that exceeded that limit
// I had to split them using the UNIX command "split", and here I want to have
// a class structure that can transparently handle both regular files
// and split files with exactly the same code, without the user having to
// specify whether a file is regular or split.
// For the time being, I will only handle the case of reading files,
// I think I do need to worry about writing files so far.
class class_file
{
public:
  // split is a flag that is true if we are dealing with a split file.
  vint8 split;
  vint8 current_index;

  char* specifications;

  // file_pointer will simply be the pointer to the file,
  // when the file is not split.  If the file is split,
  // then file_pointer will point to the current chunk of the file.
  FILE* file_pointer;

  // file_size is only used for split files, and stores the
  // length of each chunk.  Naturally, the last chunk may have
  // a smaller length than that.
  vint8 file_size;

  // this is only used for split files, and stores the
  // virtual pathname, i.e., the pathname of each of the split files
  // without the suffixes ended by the UNIX split command.
  char* pathname;

  // these functions add the right suffix to the pathname,
  // for split files.  They should probably be private
  // or protected.
  char* pathname_from_position (vint8 file_position);
  char* pathname_from_index (vint8 index);
  vint8 index_from_position(vint8 position);
  vint8 open_index (vint8 index);

  class_file (const char* filename, const char* specifications);
  ~class_file ();

  // this should be the substitute for fread.
  vint8 read (void* buffer, size_t type_size, size_t items);

  // this should be the substitute for fwrite.  For the time
  // being, this will only work for non-split files.
  vint8 write (void* buffer, size_t type_size, size_t items);

  // this should be the substitute for fseek and my 64-bit
  // equivalent.
  vint8 seek (vint8 position, int origin);

  // they should be the substitute for ftell
  vint8 tell ();
};


// these are overloads of fread, fwrite, fseek, ftell, fclose
// that work with class_file.

vint8 fread (void* buffer, size_t type_size, size_t items, class_file*
            file_pointer);
vint8 fwrite (void* buffer, size_t type_size, size_t items, 
             class_file* file_pointer);
vint8 fseek (class_file* file_pointer, long position, int origin);
vint8 function_seek(class_file* file_pointer, vint8 position, int origin);
long ftell(class_file* file_pointer);
vint8 function_tell (class_file* file_pointer);
vint8 fclose (class_file* file_pointer);

vint8 string_length (const char* argument);



class class_pixel
{
public:
  vint8 vertical;
  vint8 horizontal;

  class_pixel()
  {
    vertical = horizontal = -1;
  }

  class_pixel (vint8 in_vertical, vint8 in_horizontal)
  {
    vertical = in_vertical;
    horizontal = in_horizontal;
  }

  vint8 operator == (class_pixel & argument)
  {
    if ((vertical == argument.vertical) && (horizontal == argument.horizontal))
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }
  vint8 operator != (class_pixel & argument)
  {
    if ((vertical == argument.vertical) && (horizontal == argument.horizontal))
    {
      return 0;
    }
    else
    {
      return 1;
    }
  }
};


typedef FILE file_handle;
file_handle * open_read_file(const char * filename);
file_handle * open_save_file(const char * filename);
vint8 close_file(file_handle * file_pointer);

vint8 function_read_floats(file_handle * file_pointer, float * numbers, const vint8 size);
vint8 function_save_floats(file_handle * file_pointer, const float * numbers, const vint8 size);

vint8 function_read_doubles(file_handle * file_pointer, double * numbers, vint8 size);
vint8 function_save_doubles(file_handle * file_pointer, const double * numbers, vint8 size);







#endif    //  VASSILIS_AUXILIARIES_H
